/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2018-06-21
Description:查询工序产品成本
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx37_inq)
//-EP_SYSTEM_HEAD_END

int f_caaicx37_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;	
	int fetchRowCount	= 0;

	CString		begin_time = " ";
	CString		end_time = " ";

	CModel tcaai03("TCAAI03");


	CString  sqlstr("");
	
	
	CDbCommand cmd_inq(conn);
	
	try
	{
		begin_time  = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();		

		tcaai03.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句		
					
				sqlstr = "	SELECT	SUB_BACKLOG_CODE,SG_SIGN,MAT_THICK,sum(QTY) qty,sum(WT) WT,sum(COST_GJ) COST_GJ,sum(COST_YJ) COST_YJ,sum(COST_CJ) COST_CJ,decode(sum(WT),0,0,round(sum(COST_GJ) / sum(WT), 6)) PRICE_UNIT_GJ,decode(sum(WT),0,0,round(sum(COST_YJ) / sum(WT), 6)) PRICE_UNIT_YJ ,decode(sum(WT),0,0,round(sum(COST_CJ) / sum(WT), 6)) PRICE_UNIT_CJ "
					" FROM	tcaai03"
					" WHERE	1=1"
					" AND STATS_PERIOD>=@begin_time"
					" and STATS_PERIOD<=@end_time"
					;			
				if (tcaai03["PROD_SHIFT_GROUP"].ToString().Trim() != "") sqlstr += " AND PROD_SHIFT_GROUP = @tcaai03.PROD_SHIFT_GROUP";
				if (tcaai03["PROD_SHIFT_NO"].ToString().Trim() != "") sqlstr += " AND PROD_SHIFT_NO = @tcaai03.PROD_SHIFT_NO";
				if (tcaai03["SG_SIGN"].ToString().Trim() != "") sqlstr += " AND SG_SIGN like '%'||@tcaai03.SG_SIGN||'%'";
				if (tcaai03["SUB_BACKLOG_CODE"].ToString().Trim() != "") sqlstr += " AND SUB_BACKLOG_CODE = @tcaai03.SUB_BACKLOG_CODE";
				if (tcaai03["DEPT_CODE"].ToString().Trim() != "") sqlstr += " AND DEPT_CODE = @tcaai03.DEPT_CODE";
					sqlstr += " group by SUB_BACKLOG_CODE,SG_SIGN,MAT_THICK";
					
				
   			break;
		}  
		
		cmd_inq.SetCommandText(sqlstr);	
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("tcaai03.PROD_SHIFT_GROUP", tcaai03["PROD_SHIFT_GROUP"].ToString());
		cmd_inq.Parameters.Set("tcaai03.PROD_SHIFT_NO", tcaai03["PROD_SHIFT_NO"].ToString());
		cmd_inq.Parameters.Set("tcaai03.SG_SIGN", tcaai03["SG_SIGN"].ToString());
		cmd_inq.Parameters.Set("tcaai03.SUB_BACKLOG_CODE", tcaai03["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("tcaai03.DEPT_CODE", tcaai03["DEPT_CODE"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);				
		cmd_inq.Close();

  	 

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		 
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	
	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}