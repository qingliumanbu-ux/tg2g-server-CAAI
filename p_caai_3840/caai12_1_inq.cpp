/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 辅助材料消耗查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caai12_1_inq)
int f_caai12_1_inq(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString  sqlstr_1("");
	CString begin_time("");
	CString prod_date("");
	CString mat_code("");
	CString sub_backlog_code("");
	CString shift_group("");
	CString sg_sign("");
	CDecimal mat_thick = 0; //断面


	int i =0;


	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		prod_date         = bcls_rec->Tables[0].Rows[0]["prod_date"].ToString().Trim();
		sub_backlog_code = bcls_rec->Tables[0].Rows[0]["sub_backlog_code"].ToString().Trim();
		shift_group      = bcls_rec->Tables[0].Rows[0]["shift_group"].ToString().Trim();

		bcls_ret->Tables[0].Columns.Add(DT_STRING,"prod_date");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"mat_code");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"mat_name");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"shift_group");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL,"COMSUME_WT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"UNIT");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"REMARK");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"REGIST_MAN");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"REC_CREATOR");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"sub_backlog_code");
		bcls_ret->Tables[0].Columns.Add(DT_STRING,"sg_sign");
		bcls_ret->Tables[0].Columns.Add(DT_DECIMAL,"mat_thick");


		sqlstr = " SELECT prod_date,mat_code,mat_name,shift_group,COMSUME_WT,UNIT,REMARK,REGIST_MAN,REC_CREATOR,sub_backlog_code,sg_sign,mat_thick FROM TCAAIA10"
			"  WHERE prod_date = @prod_date "
			" and  shift_group = @shift_group "	
			" and  sub_backlog_code = @sub_backlog_code "				
			;	

		EDLog(1,1,"sqlstr=[%s]",(const char*)sqlstr);
		EDLog(1,1,"prod_date=[%s],sub_backlog_code=[%s],shift_group=[%s],sg_sign=[%s]",(const char*)prod_date,(const char*)sub_backlog_code,(const char*)shift_group,(const char*)sg_sign);

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date",prod_date);	
		cmd_inq.Parameters.Set("sub_backlog_code",sub_backlog_code);	
		cmd_inq.Parameters.Set("shift_group",shift_group);			
		cmd_inq.ExecuteReader();
		while(cmd_inq.Read())
		{
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["prod_date"]         = cmd_inq.GetString(1);
			bcls_ret->Tables[0].Rows[i]["mat_code"]          = cmd_inq.GetString(2);
			bcls_ret->Tables[0].Rows[i]["mat_name"]          = cmd_inq.GetString(3);
			bcls_ret->Tables[0].Rows[i]["shift_group"]       = cmd_inq.GetString(4);
			bcls_ret->Tables[0].Rows[i]["COMSUME_WT"]        = cmd_inq.GetDecimal(5);
			bcls_ret->Tables[0].Rows[i]["UNIT"]              = cmd_inq.GetString(6);
			bcls_ret->Tables[0].Rows[i]["REMARK"]            = cmd_inq.GetString(7);
			bcls_ret->Tables[0].Rows[i]["REGIST_MAN"]        = cmd_inq.GetString(8);
			bcls_ret->Tables[0].Rows[i]["REC_CREATOR"]       = cmd_inq.GetString(9);
			bcls_ret->Tables[0].Rows[i]["sub_backlog_code"]  = cmd_inq.GetString(10);
			bcls_ret->Tables[0].Rows[i]["sg_sign"]           = cmd_inq.GetString(11);
			bcls_ret->Tables[0].Rows[i]["mat_thick"]         = cmd_inq.GetDecimal(12);
			i++;
		}
		cmd_inq.Close(); 


		sqlstr = " SELECT @prod_date,mat_code,mat_name,@shift_group,0,MAT_UNIT,' ',' ',' ',@sub_backlog_code,' ',0  FROM TCAAC11 "
			" WHERE VALID_FLAG = '1'"
			;
		sqlstr_1 = " select FIELD_NAME from tcaac14"
			" where CODE = @sub_backlog_code"
			;
		cmd_inq1.SetCommandText(sqlstr_1);
		cmd_inq1.Parameters.Set("sub_backlog_code",sub_backlog_code);
		cmd_inq1.ExecuteReader();		
		if(cmd_inq1.Read())
		{
			sqlstr = sqlstr + " AND " +cmd_inq1.GetString(1) + "= '1'" ;
		}
		cmd_inq1.Close();

		sqlstr = sqlstr + " and mat_code not in (SELECT mat_code FROM TCAAIA10 WHERE prod_date = @prod_date and shift_group = @shift_group  and  sub_backlog_code = @sub_backlog_code 	";

		sqlstr =sqlstr + ")  order BY  mat_code " ;

		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("prod_date",prod_date);	
		cmd_inq.Parameters.Set("sub_backlog_code",sub_backlog_code);	
		cmd_inq.Parameters.Set("shift_group",shift_group);			
		cmd_inq.ExecuteReader();
		while(cmd_inq.Read())
		{
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[i]["prod_date"]         = cmd_inq.GetString(1);
			bcls_ret->Tables[0].Rows[i]["mat_code"]          = cmd_inq.GetString(2);
			bcls_ret->Tables[0].Rows[i]["mat_name"]          = cmd_inq.GetString(3);
			bcls_ret->Tables[0].Rows[i]["shift_group"]       = cmd_inq.GetString(4);
			bcls_ret->Tables[0].Rows[i]["COMSUME_WT"]        = cmd_inq.GetDecimal(5);
			bcls_ret->Tables[0].Rows[i]["UNIT"]              = cmd_inq.GetString(6);
			bcls_ret->Tables[0].Rows[i]["REMARK"]            = cmd_inq.GetString(7);
			bcls_ret->Tables[0].Rows[i]["REGIST_MAN"]        = cmd_inq.GetString(8);
			bcls_ret->Tables[0].Rows[i]["REC_CREATOR"]       = cmd_inq.GetString(9);
			bcls_ret->Tables[0].Rows[i]["sub_backlog_code"]  = cmd_inq.GetString(10);
			bcls_ret->Tables[0].Rows[i]["sg_sign"]           = cmd_inq.GetString(11);
			bcls_ret->Tables[0].Rows[i]["mat_thick"]         = cmd_inq.GetDecimal(12);
			i++;
		}
		cmd_inq.Close(); 

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
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
	return doFlag;
}

