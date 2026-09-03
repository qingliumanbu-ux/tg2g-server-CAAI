/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2018-06-21
Description:查询关键指标查询
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caaicx35_inq)
//-EP_SYSTEM_HEAD_END

int f_caaicx35_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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
		//系统的分页类信息。
		CPageInfo pageInfo;

		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();

		tcaai03.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		sqlstr = " select mat_code,mat_name,mat_unit,UNIT_PRICE,bz_wt,UNIT_WT,UNIT_COST,best_wt,best_cost"
			",best_wt-bz_wt as BEST_DIF_WT, round(UNIT_PRICE*(best_wt-bz_wt),6) AS BEST_DIF_cost"
			",unit_wt - bz_wt as DIF_WT, round(UNIT_PRICE*(unit_wt-bz_wt),6) as DIF_COST"
			" FROM ("
			" select t1.mat_code,t1.mat_name,t1.mat_unit,decode(best_proc_wt,0,0,round(best_wt/best_proc_wt,6)) best_wt,decode(best_proc_wt,0,0,round(best_cost/best_proc_wt,6)) best_cost"
			",decode(proc_wt,0,0,round(wt/proc_wt,6)) UNIT_WT,decode(proc_wt,0,0,round(cost/proc_wt,6)) UNIT_COST"
			",CASE WHEN wt!=0 AND COST!=0 THEN ROUND(COST/WT,6) WHEN WT=0 AND best_wt!=0 AND BEST_COST !=0 THEN ROUND(BEST_COST/best_wt,6) ELSE 0 END UNIT_PRICE"
			" ,bz_wt"
			" FROM "
			" (select mat_code,mat_name,MAT_UNIT"
			" from tcaac11"
			" where MAT_CODE IN (SELECT MAT_CODE FROM TCAAC03 WHERE SUB_bACKLOG_CODE =@sub_backlog_code)"
			" AND USE_FLAG_1 ='1') t1"
			" left join "
			" (select mat_code,equ_no,sum(WT) best_wt,sum(COST_GJ) best_cost,sum(case when stats_period>=@begin_time then WT else 0 end) wt,sum(case when stats_period>=@begin_time then COST_GJ else 0 end) cost "
			" from tcaai04"
			" where 1=1"
			" and SUB_BACKLOG_CODE = @sub_backlog_code"
			" and stats_period >= to_char(add_months(to_date(@begin_time,'yyyyMMdd hh24:mi:ss'),-24),'yyyyMMdd')"
			" and stats_period<= @end_time"
			" group by mat_code,equ_no ) t2"
			" on t1.mat_code = t2.mat_code"
			" left join "
			" (select equ_no,sum(wt) best_proc_wt,sum(case when stats_period>=@begin_time then wt else 0 end) proc_wt "
			" from tcaai03"
			" where 1=1"
			" and equ_no = @equ_no"
			" and stats_period >= to_char(add_months(to_date(@begin_time,'yyyyMMdd hh24:mi:ss'),-24),'yyyyMMdd')"
			" and stats_period<= @end_time"
			" group by equ_no ) t3"
			" on t2.equ_no = t3.equ_no"
			" left join "
			"( select mat_code,avg(use_unit) bz_wt from tcaac05 where sub_backlog_code =@sub_backlog_code"
			" group by mat_code ) t4"
			" on t1.mat_code = t4.mat_code"
			" )"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time.SubstringNE(0,8));
		cmd_inq.Parameters.Set("end_time", end_time.SubstringNE(0, 8));
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai03["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("equ_no", tcaai03["EQU_NO"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		//Log::Trace("", "", "str={0}", str);
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